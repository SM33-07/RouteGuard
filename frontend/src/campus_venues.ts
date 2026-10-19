/**
 * RouteGuard — MUJ Campus Verified Venue Geographic Layout
 *
 * Provides calibrated [longitude, latitude] coordinates for the 13 C-owned campus venues.
 * Coordinates are mapped around the central MUJ campus benchmark (approx. 75.565234, 26.843855)
 * using Google Maps satellite reference imagery (docs/reference, public/maps) and official master plans.
 *
 * VENUE ACCURACY BOUNDARY:
 * Connections are synthetic/modeled for CSC2101/CSC2130 DSA graph demonstration.
 * They do NOT represent verified pedestrian sidewalks, municipal transit, or emergency paths.
 */

import type { FeatureCollection, Point } from 'geojson';

export interface CampusVenueGeo {
  id: number;
  label: string;
  fullName: string;
  coordinates: [number, number]; // [longitude, latitude] (WGS 84 / GeoJSON format)
  category: 'arrival' | 'academic' | 'dining' | 'sports' | 'library' | 'hub';
  landmarkIndex: number | null; // Corresponding 1-8 marker on schematic, if applicable
  confidence: 'verified_poi' | 'approximate';
  notes: string;
}

export const MUJ_CAMPUS_CENTER: [number, number] = [75.56545, 26.84360]; // [lng, lat]
export const MUJ_DEFAULT_ZOOM = 16.3;

export const CAMPUS_VENUES: CampusVenueGeo[] = [
  {
    id: 0,
    label: 'Main Gate',
    fullName: 'Main Gate — Primary campus arrival & visitor entry',
    coordinates: [75.56386, 26.84158],
    category: 'arrival',
    landmarkIndex: null,
    confidence: 'verified_poi',
    notes: 'Primary western campus entrance where the access road meets the campus boundary (OSM University Entrance). Distinct from the separate southern VIP/Faculty Gate near the parking area.',
  },
  {
    id: 1,
    label: 'Dome Building',
    fullName: 'Dome Building — University administrative focal point & central dome',
    coordinates: [75.56565, 26.84165],
    category: 'academic',
    landmarkIndex: 1,
    confidence: 'verified_poi',
    notes: 'Iconic central dome building of MUJ, aligned directly with the large horizontal building footprint labelled Administrative Block on the basemap.',
  },
  {
    id: 2,
    label: 'Grand Stairs',
    fullName: 'Grand Stairs / Central Plaza — Central arrival and circulation hub',
    coordinates: [75.56550, 26.84252],
    category: 'hub',
    landmarkIndex: null,
    confidence: 'verified_poi',
    notes: 'Grand staircase and central circulation plaza immediately north/above the Dome Building / Administrative Block.',
  },
  {
    id: 3,
    label: 'Academic Block 1',
    fullName: 'Academic Block 1 — West quadrangle complex (below AB3)',
    coordinates: [75.56435, 26.84275],
    category: 'academic',
    landmarkIndex: 2,
    confidence: 'verified_poi',
    notes: 'Major teaching complex with courtyard, situated left/west of Grand Stairs and below Academic Block 3.',
  },
  {
    id: 4,
    label: 'Academic Block 2',
    fullName: 'Academic Block 2 — East teaching and engineering quadrangle',
    coordinates: [75.56620, 26.84295],
    category: 'academic',
    landmarkIndex: 3,
    confidence: 'verified_poi',
    notes: 'Major academic quadrangle east of the central axis, near the geographic location where Dome Building was previously placed.',
  },
  {
    id: 5,
    label: 'Old Mess / Food Court',
    fullName: 'Old Mess / Food Court — Central dining and student commons',
    coordinates: [75.56530, 26.84296],
    category: 'dining',
    landmarkIndex: 4,
    confidence: 'verified_poi',
    notes: 'Central campus dining facility located along the central pedestrian axis immediately above Grand Stairs.',
  },
  {
    id: 6,
    label: 'Academic Block 3',
    fullName: 'Academic Block 3 — North-west academic building (above AB1)',
    coordinates: [75.56425, 26.84370],
    category: 'academic',
    landmarkIndex: 6,
    confidence: 'verified_poi',
    notes: 'Upper-left academic building complex indicated in the annotated reference, placed above Academic Block 1.',
  },
  {
    id: 7,
    label: 'Lecture Hall Complex',
    fullName: 'Lecture Hall Complex — Tiered auditorium-style lecture halls (LHC)',
    coordinates: [75.56470, 26.84432],
    category: 'academic',
    landmarkIndex: 5,
    confidence: 'verified_poi',
    notes: 'Tiered lecture halls complex located north of Old Mess. Displays full name Lecture Hall Complex in UI and venue index.',
  },
  {
    id: 8,
    label: 'Workshops / Labs',
    fullName: 'Workshops / Labs — Eastern engineering and research workshops',
    coordinates: [75.56696, 26.84365],
    category: 'academic',
    landmarkIndex: 8,
    confidence: 'verified_poi',
    notes: 'Marked large building complex on the right housing engineering and research workshops.',
  },
  {
    id: 9,
    label: 'Football Ground',
    fullName: 'Football Ground — Campus sports turf and athletic field',
    coordinates: [75.56580, 26.84445],
    category: 'sports',
    landmarkIndex: 7,
    confidence: 'verified_poi',
    notes: 'Regulation sports turf located at the exact point indicated by the arrow in the reference, east of Lecture Hall Complex.',
  },
  {
    id: 10,
    label: 'Cricket Ground',
    fullName: 'Cricket Ground — Northern circular cricket field',
    coordinates: [75.56416, 26.84554],
    category: 'sports',
    landmarkIndex: 7,
    confidence: 'verified_poi',
    notes: 'Oval cricket field indicated in the annotated reference forming the northern athletic perimeter.',
  },
  {
    id: 11,
    label: 'Outdoor Courts',
    fullName: 'Outdoor Courts — Basketball, tennis & volleyball courts',
    coordinates: [75.56545, 26.84570],
    category: 'sports',
    landmarkIndex: 7,
    confidence: 'verified_poi',
    notes: 'Marked outdoor-courts complex (tennis, badminton, volleyball) east of Cricket Ground.',
  },
  {
    id: 12,
    label: 'Knowledge Centre',
    fullName: 'Knowledge Centre — Digital library & resource centre',
    coordinates: [75.56660, 26.84230],
    category: 'library',
    landmarkIndex: null,
    confidence: 'verified_poi',
    notes: 'Dedicated digital library and resource center situated to the right/east of Dome Building / Administrative Block.',
  },
];

/**
 * Returns GeoJSON FeatureCollection of venue points
 */
export function getCampusVenuesGeoJSON(): FeatureCollection<Point> {
  return {
    type: 'FeatureCollection',
    features: CAMPUS_VENUES.map((v) => ({
      type: 'Feature',
      id: v.id,
      properties: {
        id: v.id,
        label: v.label,
        fullName: v.fullName,
        category: v.category,
        landmarkIndex: v.landmarkIndex,
        confidence: v.confidence,
        notes: v.notes,
      },
      geometry: {
        type: 'Point',
        coordinates: v.coordinates,
      },
    })),
  };
}
